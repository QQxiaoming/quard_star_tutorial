(: Name: orderedunorderedexpr-4:)
(: Description: Evaluation of ordered expression used with quantified expression.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

ordered {if (some $x in (1, 2, 3), $y in (2, 3, 4) satisfies $x + $y = 4) then (0,1,2,3,4) else ("a","b")}