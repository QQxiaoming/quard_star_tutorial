(: Name: emptyorderdecl-22 :)
(: Description: Evaluation of empty order declaration set to "empty greatest" and a set of nodes (one of them set to empty) sorted in ascending order and local order by :)
(: overrides empty order declaration in prolog (empty greatest.:)

declare default order empty greatest;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (<a>1</a>,<a>4</a>,<a></a>,<a>7</a>)
order by zero-or-one($i/text()) ascending empty greatest
return $i
