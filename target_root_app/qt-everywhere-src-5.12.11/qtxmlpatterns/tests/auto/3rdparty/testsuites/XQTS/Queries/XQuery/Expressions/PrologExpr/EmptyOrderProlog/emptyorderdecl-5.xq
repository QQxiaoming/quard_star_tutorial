(: Name: emptyorderdecl-5 :)
(: Description: Evaluation of a prolog that specifies "empty greatest" with a set of numbers (two that results in NaN) and sort them in ascending order :)

declare default order empty greatest;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (1,4,0 div 0E0,0 div 0E0,7)
order by $i ascending
return $i