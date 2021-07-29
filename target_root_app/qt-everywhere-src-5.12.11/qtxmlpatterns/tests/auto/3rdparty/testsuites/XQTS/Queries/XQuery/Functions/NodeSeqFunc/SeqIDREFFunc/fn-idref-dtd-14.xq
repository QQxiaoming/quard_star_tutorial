(: Name: fn-idref-dtd-14 :)
(: Description: Evaluation of fn:idref used as part of a node expression ("is" operand). :)
(: Compare different elements.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:idref("id1", $input-context1/IDS[1])) is (fn:idref("id2", $input-context1/IDS[1]))