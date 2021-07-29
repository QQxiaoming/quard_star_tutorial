(: name : orderbywithout-10 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWOR expression set to "count($x) ", where $x is a set of node with strings as content.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results> {
for $x in ("A String","B String","C String","D String","E String","F String","G String","H String","I String","J String","K String","L String","M String","N String","O String","P String","R String","S String","T String","U String","V String","W String","X String","Y String","Z String")
 order by count(xs:string($x)) return count(xs:string($x))
}
</results>
