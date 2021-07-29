(: Name: orderedunorderedexpr-5:)
(: Description: Evaluation of ordered expression used with quantified expression.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

ordered {if (every $x in (1, 2, 3) satisfies $x < 4) then (0,1,2,3,4) else ("a","b")}