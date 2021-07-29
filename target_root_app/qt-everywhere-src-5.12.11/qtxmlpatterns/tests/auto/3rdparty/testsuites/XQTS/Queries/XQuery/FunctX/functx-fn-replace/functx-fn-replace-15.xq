(:**************************************************************:)
(: Test: functx-fn-replace-15                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(replace('Chap 2...Chap 3...Chap 4...',
        'Chap (\d)', 'Sec $1.0'))
