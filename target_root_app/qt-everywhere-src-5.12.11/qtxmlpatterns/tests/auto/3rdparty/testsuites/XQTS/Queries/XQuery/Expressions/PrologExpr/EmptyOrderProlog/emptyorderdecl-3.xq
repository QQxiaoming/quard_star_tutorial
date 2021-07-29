(: Name: emptyorderdecl-3 :)
(: Description: Evaluation of a prolog that specifies "empty greatest" with a set of nodes (two empty) and sort them in ascending order :)

declare default order empty greatest;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (<a>1</a>,<a>4</a>,<a></a>,<a></a>,<a>7</a>)
order by zero-or-one($i/text()) ascending
return $i
