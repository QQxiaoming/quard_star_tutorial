(: Name: fn-idref-dtd-5 :)
(: Description: Evaluation of fn:idref with given ID matching a single element. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{fn:idref("id1",$input-context1/IDS[1])}</results>