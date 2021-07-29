(: Name: fn-idref-dtd-21 :)
(: Description: Evaluation of fn:idref for which $arg uses the fn:string-join function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{fn:idref(fn:string-join(("id","1"),""), exactly-one($input-context1/IDS[1]))}</results>
