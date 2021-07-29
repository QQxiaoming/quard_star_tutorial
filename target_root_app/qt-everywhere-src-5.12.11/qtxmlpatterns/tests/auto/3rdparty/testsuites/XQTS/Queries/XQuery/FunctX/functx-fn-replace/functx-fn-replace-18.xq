(:**************************************************************:)
(: Test: functx-fn-replace-18                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(replace('2006-10-18',
        '\d{2}(\d{2})-(\d{2})-(\d{2})',
        '$2/$3/$1'))
