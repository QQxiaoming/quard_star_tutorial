(: Name: emptyorderdecl-16 :)
(: Description: Evaluation of empty order declaration set to "empty least" and a set of numbers (one set to Nan Expression) sorted in ascending order.:)

declare default order empty least;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (1,4,0 div 0E0,7)
order by $i ascending
return $i