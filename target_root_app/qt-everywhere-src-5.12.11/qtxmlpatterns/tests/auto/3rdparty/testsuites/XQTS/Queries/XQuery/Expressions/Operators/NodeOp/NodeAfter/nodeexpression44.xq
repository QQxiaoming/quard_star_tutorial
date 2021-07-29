
(:*******************************************************:)
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: January 18, 2005                                :)
(:Purpose: Test of a ComparisonExpression      :)
(:with the operands set as follows          :)
(:  operand1 = Single Node Element:)
(:  operator = >>:)
(:  operand2 = Sequence of single Element Node:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

$input-context1/works[1]/employee[1]/empnum[1] >> ($input-context2/staff[1]/employee[1]/empnum[1])
