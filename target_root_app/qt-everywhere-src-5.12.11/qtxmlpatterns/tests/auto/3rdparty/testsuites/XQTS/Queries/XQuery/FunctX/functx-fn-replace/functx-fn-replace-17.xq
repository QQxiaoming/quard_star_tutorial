(:**************************************************************:)
(: Test: functx-fn-replace-17                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(replace('2315551212',
        '(\d{3})(\d{3})(\d{4})', '($1) $2-$3'))
