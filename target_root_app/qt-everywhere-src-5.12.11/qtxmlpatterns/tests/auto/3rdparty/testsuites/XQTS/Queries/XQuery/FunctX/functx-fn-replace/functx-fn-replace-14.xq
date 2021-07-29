(:**************************************************************:)
(: Test: functx-fn-replace-14                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(replace('aaaah', 'a{2,3}?', 'X'))
