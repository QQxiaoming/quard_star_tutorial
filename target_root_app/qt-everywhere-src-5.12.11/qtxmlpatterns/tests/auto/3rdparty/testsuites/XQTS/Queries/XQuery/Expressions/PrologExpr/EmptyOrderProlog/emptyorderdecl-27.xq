(: Name: emptyorderdecl-27 :)
(: Description: Evaluation of empty order declaration set to "empty least" and a set of numbers (one of them set to a NaN expression) sorted in ascending order and local order by :)
(: that overriddes empty order declaration in prolog. :)

declare default order empty least;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (1,4,3,0 div 0E0,7)
order by $i ascending empty greatest
return $i