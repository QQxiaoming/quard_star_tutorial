(: Name: fn-idref-dtd-13 :)
(: Description: Evaluation of fn:idref used as part of a node expression ("is" operand). :)
(: Compare same elements.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:idref("id1", $input-context1/IDS[1])) is (fn:idref("id1", $input-context1/IDS[1]))