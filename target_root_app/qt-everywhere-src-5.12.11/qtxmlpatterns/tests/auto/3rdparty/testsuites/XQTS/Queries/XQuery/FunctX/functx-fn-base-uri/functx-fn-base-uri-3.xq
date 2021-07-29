(:**************************************************************:)
(: Test: functx-fn-base-uri-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


let $cats := doc($input-context1)
return (base-uri($cats//catalog[2]/product/@href))
