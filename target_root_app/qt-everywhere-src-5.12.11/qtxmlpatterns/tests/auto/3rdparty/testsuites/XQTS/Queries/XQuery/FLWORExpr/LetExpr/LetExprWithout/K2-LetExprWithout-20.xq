(:*******************************************************:)
(: Test: K2-LetExprWithout-20                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Bind to let and subsequently use the for expression. :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)
    
let $emps := $input-context1//employee[location = "Denver"]

for $d in distinct-values($emps/deptno)
let $e := $emps[deptno = $d]
return <dept>
    <deptno>{$d}</deptno>
    <headcount> {count($e)} </headcount>
    <payroll> {sum($e/salary)} </payroll>
</dept>