(: Name: fn-idref-dtd-18 :)
(: Description: Evaluation of fn:idref for which ID uses the fn:upper-case function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{fn:idref(fn:upper-case("id5"), exactly-one($input-context1/IDS[1]))}</results>
