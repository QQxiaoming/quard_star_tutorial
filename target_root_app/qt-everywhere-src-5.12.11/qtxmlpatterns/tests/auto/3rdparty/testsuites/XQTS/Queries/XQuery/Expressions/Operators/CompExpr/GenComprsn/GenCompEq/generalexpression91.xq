
(:*******************************************************:)
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 2, 2005                                :)
(:Purpose: Test of a General Expression      :)
(:with the operands set as follows          :)
(:  operand1 = Sequence of multiple element nodes (multiple sources):)
(:  operator = =:)
(:  operand2 = Atomic Value:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

($input-context1/works/employee[1]/hours[1],$input-context2/staff/employee[6]/hours[1]) = 10000
