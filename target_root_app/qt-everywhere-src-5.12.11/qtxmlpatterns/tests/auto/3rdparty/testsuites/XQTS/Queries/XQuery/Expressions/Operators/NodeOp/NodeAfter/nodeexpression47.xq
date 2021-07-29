(:*******************************************************:)
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 24, 2005                                    :)
(:Name nodeexpression47                                  :)
(:Purpose: Test of a ComparisonExpression testing the    :)
(: rule that states "If any node in a given tree, T1,    :)
(: occurs before any node in a different tree, T2, then  :)
(: all nodes in T1 are before all nodes in T2." Compare  :)
(: various nodes of the first operand against various   :)
(: nodes of the second operand.                          :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

(($input-context1/works[1]/employee[1]/empnum[1] >> $input-context2/staff[1]/employee[1]/empnum[1])
and ($input-context1/works[1]/employee[2]/empnum[1] >> $input-context2/staff[1]/employee[2]/empnum[1])
and ($input-context1/works[1]/employee[3]/empnum[1] >> $input-context2/staff[1]/employee[3]/empnum[1]))
or
(($input-context1/works[1]/employee[1]/empnum[1] << $input-context2/staff[1]/employee[1]/empnum[1])
and ($input-context1/works[1]/employee[2]/empnum[1] << $input-context2/staff[1]/employee[2]/empnum[1])
and ($input-context1/works[1]/employee[3]/empnum[1] << $input-context2/staff[1]/employee[3]/empnum[1]))
