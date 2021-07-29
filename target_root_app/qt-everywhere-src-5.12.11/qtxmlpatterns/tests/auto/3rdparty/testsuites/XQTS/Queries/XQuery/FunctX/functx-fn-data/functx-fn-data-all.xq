(:**************************************************************:)
(: Test: functx-fn-data-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


let $cat := doc($input-context1)
return (data($cat//product[1]/number), data($cat//number), data($cat//product[1]/@dept), data($cat//product[1]/colorChoices), data($cat//product[1]), data($cat//product[4]/desc))