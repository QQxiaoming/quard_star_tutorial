(: Name: fn-idref-dtd-16 :)
(: Description: Evaluation of fn:idref for which ID list have the same value for all its members but different case. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{fn:idref(("id1","ID1"), exactly-one($input-context1/IDS[1]))}</results>
