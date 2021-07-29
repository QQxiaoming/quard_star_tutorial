(:**************************************************************:)
(: Test: functx-fn-tokenize-8                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(tokenize(
   '2006-12-25T12:15:00', '[\-T:]'))
