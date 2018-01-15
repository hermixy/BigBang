#lang racket

(provide (all-defined-out))

(define detect-language
  (lambda [languages path]
    (for/or ([l.px.clr (in-list languages)])
      (and (regexp-match? (vector-ref l.px.clr 2) path)
           (vector-ref l.px.clr 0)))))

(define detect-midnight
  (lambda [ts]
    (define the-day (seconds->date ts))
    (- ts (+ ;(* (date-week-day the-day) 86400)
             (* (date-hour the-day) 3600)
             (* (date-minute the-day) 60)
             (date-second the-day)))))

(define language-statistics
  (lambda [sln-root languages excludes]
    (define (use-dir? dir)
      (not (ormap (curryr regexp-match? dir)
                  (cons #px"/(.git|compiled)/?$" excludes))))
    (define statistics (make-hasheq))
    (for ([path (in-directory sln-root use-dir?)]
          #:when (file-exists? path))
      (define language (detect-language languages path))
      (when (symbol? language)
        (hash-set! statistics language
                   (+ (hash-ref statistics language (λ [] 0))
                      (file-size path)))))
    statistics))

(define git-log-shortstat
  (lambda []
    (define statistics (make-hasheq))
    (define-values (&insertion &deletion) (values (box 0) (box 0)))
    (define git (find-executable-path "git"))
    (with-handlers ([exn? void])
      (define (stat++ ts tokens)
        (define midnight (detect-midnight ts))
        (define-values ($insertion $deletion)
          (cond [(= (length tokens) 4) (values (car tokens) (caddr tokens))]
                [(char-ci=? (string-ref (cadr tokens) 0) #\i) (values (car tokens) "0")]
                [else (values "0" (car tokens))]))
        (define-values (insertion deletion) (values (string->number $insertion) (string->number $deletion)))
        (set-box! &insertion (+ insertion (unbox &insertion)))
        (set-box! &deletion (+ deletion (unbox &deletion)))
        (hash-set! statistics midnight
                   (let ([stat0 (hash-ref statistics midnight (λ [] (cons 0 0)))])
                     (cons (+ (car stat0) insertion) (+ (cdr stat0) deletion)))))
      (parameterize ([current-custodian (make-custodian)]
                     [current-subprocess-custodian-mode 'kill])
        (define-values (git-log /dev/gitin _out _err)
          (subprocess #false #false #false 'new
                      git "log" "--pretty=format:%at" "--shortstat" "--no-merges"))
        (with-handlers ([exn? void])
          (let shortstat ()
            (define timestamp (read-line /dev/gitin))
            (define tokens (string-split (read-line /dev/gitin)))
            (read-line /dev/gitin) ; skip-empty-line 
            (stat++ (string->number timestamp) (cdddr tokens))
            (shortstat)))
        (custodian-shutdown-all (current-custodian))))

    (values statistics (unbox &insertion) (unbox &deletion))))

(define git-log-numstat
  (lambda [languages excludes]
    (define statistics (make-hasheq))
    (define insertions (make-hasheq))
    (define deletions (make-hasheq))
    (define git (find-executable-path "git"))
    (with-handlers ([exn? void])
      (define (use-dir? dir) (not (ormap (curryr regexp-match? dir) excludes)))
      (define (stat++ ts stats)
        (define midnight (detect-midnight ts))
        (for ([stat (in-list stats)])
          (define path (caddr stat))
          (when (use-dir? path)
            (define language (detect-language languages path))
            (when (symbol? language)
              (define lang-stats (hash-ref! statistics language (λ [] (make-hasheq))))
              (define-values (insertion deletion) (values (car stat) (cadr stat)))
              (hash-set! insertions language (+ insertion (hash-ref insertions language (λ [] 0))))
              (hash-set! deletions language (+ deletion (hash-ref deletions language (λ [] 0))))
              (hash-set! lang-stats midnight
                         (let ([stat0 (hash-ref lang-stats midnight (λ [] (cons 0 0)))])
                           (cons (+ (car stat0) insertion) (+ (cdr stat0) deletion))))))))
      (parameterize ([current-custodian (make-custodian)]
                     [current-subprocess-custodian-mode 'kill])
        (define-values (git-log /dev/gitin _out _err)
          (subprocess #false #false #false 'new
                      git "log" "--pretty=format:%at" "--numstat" "--no-merges"))
        (with-handlers ([exn? void])
          (let pretty-numstat ()
            (define timestamp (read-line /dev/gitin))
            (define tokens (string-split (read-line /dev/gitin)))
            (let numstat ([stats null])
              (define tokens (string-split (read-line /dev/gitin)))
              (cond [(= (length tokens) 3)
                     (let ([stat (list (string->number (car tokens)) (string->number (cadr tokens)) (caddr tokens))])
                       (numstat (cond [(and (car stat) (cadr stat)) (cons stat stats)]
                                      [else stats])))]
                    [else (stat++ (string->number timestamp) (reverse stats))
                          (pretty-numstat)]))))
        (custodian-shutdown-all (current-custodian))))

    (values statistics insertions deletions)))