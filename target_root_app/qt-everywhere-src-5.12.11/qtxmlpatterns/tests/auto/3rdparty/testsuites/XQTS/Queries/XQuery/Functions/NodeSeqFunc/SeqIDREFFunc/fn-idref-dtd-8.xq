(: Name: fn-idref-dtd-8 :)
(: Description: Evaluation of fn:idref function, which attempts to create element with :)
(: two attributes with same name.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{fn:idref(("id1", "id2"), $input-context1/IDS[1])}</results>