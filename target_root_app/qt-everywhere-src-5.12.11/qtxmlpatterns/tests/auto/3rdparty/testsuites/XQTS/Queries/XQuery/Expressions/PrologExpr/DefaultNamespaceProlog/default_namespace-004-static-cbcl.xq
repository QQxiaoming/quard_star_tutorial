(:*******************************************************:)
(: Test: default_namespace-004.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://example.org";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function summary($emps as element(employee)*)
                as element(dept)*
{
  for $d in fn:distinct-values($emps/deptno)
  let $e := $emps[fn:string(fn:exactly-one(deptno)) = fn:string($d)]
  return
    <dept>
      <deptno>{$d}</deptno>
      <headcount> {fn:count($e)} </headcount>
       <payroll> {fn:sum($e/(fn:string(fn:exactly-one(salary)) cast as xs:double))} </payroll>
     </dept>
};
summary($input-context//employee[fn:string(fn:exactly-one(location)) = "Denver"])
