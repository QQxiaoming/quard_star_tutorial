(: Name: fn-idref-dtd-17 :)
(: Description: Evaluation of fn:idref for which ID uses the fn lower case function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{fn:idref(fn:lower-case("ID1"), exactly-one($input-context1/IDS[1]))}</results>
