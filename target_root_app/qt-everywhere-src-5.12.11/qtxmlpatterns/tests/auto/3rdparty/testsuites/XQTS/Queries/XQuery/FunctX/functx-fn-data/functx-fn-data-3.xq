(:**************************************************************:)
(: Test: functx-fn-data-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


let $cat := doc($input-context1)
return (data($cat//product[1]/@dept))
