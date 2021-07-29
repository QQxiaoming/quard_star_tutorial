(: Name: fn-idref-dtd-14 :)
(: Description: Evaluation of fn:idref used as part of a node expression ("is" operand). :)
(: Compare different elements.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one(fn:idref("id1", exactly-one($input-context1/IDS[1]))) is exactly-one(fn:idref("id2", exactly-one($input-context1/IDS[1])))
