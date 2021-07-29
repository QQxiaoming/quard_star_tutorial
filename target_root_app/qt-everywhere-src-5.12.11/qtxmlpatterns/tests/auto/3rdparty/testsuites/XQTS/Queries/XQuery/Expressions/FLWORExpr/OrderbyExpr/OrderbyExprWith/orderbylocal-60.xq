(: name : orderbylocal-60 :)
(: description : Evaluation of "order by" clause with an unknown collation. :)
(: Use a locally defined sequence :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


for $x in ("A","B","C")
 order by string($x) ascending collation "http://nonexistentcollition.org/ifsupportedwoooayouarethebestQueryimplementation/makeitharder" return string($x)
