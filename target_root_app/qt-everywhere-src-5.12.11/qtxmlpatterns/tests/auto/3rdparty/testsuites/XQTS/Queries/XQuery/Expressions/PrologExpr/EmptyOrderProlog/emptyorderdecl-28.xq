(: Name: emptyorderdecl-28 :)
(: Description: Evaluation of empty order declaration set to "empty least" and a set of nodes (one of them set to empty) sorted in ascending order and local order by :)
(: clause set to the same value.:)

declare default order empty least;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (<a>1</a>,<a>4</a>,<a></a>,<a>7</a>)
order by $i/text() ascending empty least
return $i