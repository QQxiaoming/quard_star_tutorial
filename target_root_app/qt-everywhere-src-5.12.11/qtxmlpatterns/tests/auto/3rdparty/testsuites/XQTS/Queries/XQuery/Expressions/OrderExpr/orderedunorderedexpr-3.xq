(: Name: orderedunorderedexpr-3:)
(: Description: Evaluation of ordered expression used with "or" expression ("or" operator).:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

ordered {if (1 eq 1 or 2 eq 3) then (0,1,2,3,4) else ("a","b")}