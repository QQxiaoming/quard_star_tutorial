(: Name: emptyorderdecl-7 :)
(: Description: Evaluation of a prolog that specifies "empty least" with a set of nodes (one empty) and no order by clause :)

declare default order empty least;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (<a>1</a>,<a>4</a>,<a></a>,<a>7</a>)
return $i