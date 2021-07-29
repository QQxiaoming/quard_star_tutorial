(: Name: emptyorderdecl-15 :)
(: Description: Evaluation of empty order declaration set to "empty least" and a set of nodes (two of them empty) sorted in ascending order.:)

declare default order empty least;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (<a>1</a>,<a>4</a>,<a></a>,<a></a>,<a>7</a>)
order by zero-or-one($i/text()) ascending
return $i
