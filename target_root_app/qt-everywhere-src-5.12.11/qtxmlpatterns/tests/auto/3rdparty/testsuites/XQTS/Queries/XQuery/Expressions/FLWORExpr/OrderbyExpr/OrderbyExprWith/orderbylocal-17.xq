(: name : orderbylocal-17 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWR expression set to "string-length($x) ", where $x is a set of Strings and the ordering mode set to ascending :)
(: Use a locally defined sequence. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results> {
for $x in("A String","B String","C String","D String","E String","F String","G String","H String","I String","J String","K String","L String","M String","N String","O String","P String","R String","S String","T String","U String","V String","W String","X String","Y String","Z String")
 order by string-length(xs:string($x)) ascending return string-length(xs:string($x))
}
</results>
