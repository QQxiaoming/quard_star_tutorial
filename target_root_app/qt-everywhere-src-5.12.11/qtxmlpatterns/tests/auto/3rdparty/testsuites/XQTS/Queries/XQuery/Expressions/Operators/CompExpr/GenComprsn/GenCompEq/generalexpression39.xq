
(:*******************************************************:)
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 2, 2005                                :)
(:Purpose: Test of a General Expression      :)
(:with the operands set as follows          :)
(:  operand1 = Sequence of single atomic values:)
(:  operator = =:)
(:  operand2 = Sequence of multiple element nodes (multiple sources):)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

(10000,50000) = ($input-context1/works/employee[1]/hours[1],$input-context2/staff/employee[6]/grade[1])
