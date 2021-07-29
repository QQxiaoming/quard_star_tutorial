(: Name: fn-idref-dtd-9 :)
(: Description: Evaluation of fn:idref with multiple ID, but only one matching one element. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<result>{fn:idref(("id1", "nomatching"), exactly-one($input-context1/IDS[1]))}</result>
