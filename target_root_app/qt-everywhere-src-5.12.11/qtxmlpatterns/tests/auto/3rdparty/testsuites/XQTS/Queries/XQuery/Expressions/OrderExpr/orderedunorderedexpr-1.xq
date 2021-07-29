(: Name: orderedunorderedexpr-1:)
(: Description: Evaluation of ordered expression together with if expression ("some" operator).:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

ordered {if (fn:true()) then (0,1,2,3,4) else ("A","B","C")}