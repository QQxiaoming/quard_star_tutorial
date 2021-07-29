(:********************************************************************:)
(: Test: function-declaration-004.xq                                  :)
(: Written By: Pulkita Tyagi                                          :)
(: Date: Thu Jun  2 00:24:56 2005                                     :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function local:summary($emps as element(employee,xs:untyped)*)
                as element(dept)*
{
  for $d in distinct-values($emps/deptno)
  let $e := $emps[deptno = $d]
  return
    <dept>
      <deptno>{$d}</deptno>
      <headcount> {count($e)} </headcount>
       <payroll> {sum($e/salary)} </payroll>
     </dept>
};
local:summary($input-context//employee[location = "Denver"])
