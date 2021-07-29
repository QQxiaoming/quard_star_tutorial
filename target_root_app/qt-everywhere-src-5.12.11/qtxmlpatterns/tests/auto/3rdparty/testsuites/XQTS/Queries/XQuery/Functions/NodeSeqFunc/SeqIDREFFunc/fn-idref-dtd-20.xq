(: Name: fn-idref-dtd-20 :)
(: Description: Evaluation of fn:idref for which $arg uses the xs:string function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{fn:idref(xs:string("id1"), $input-context1/IDS[1])}</results>