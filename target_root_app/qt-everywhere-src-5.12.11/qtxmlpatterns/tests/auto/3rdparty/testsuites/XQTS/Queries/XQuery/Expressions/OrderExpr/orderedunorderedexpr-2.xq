(: Name: orderedunorderedexpr-2:)
(: Description: Evaluation of ordered expression used with "or" expression ("and" operator).:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

ordered {if (1 eq 1 and 2 eq 2) then (0,1,2,3,4) else ("a","b")}