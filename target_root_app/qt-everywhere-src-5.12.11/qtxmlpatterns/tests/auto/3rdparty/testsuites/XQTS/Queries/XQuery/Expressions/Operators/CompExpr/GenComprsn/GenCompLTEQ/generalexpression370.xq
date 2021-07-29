
(:*******************************************************:)
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 2, 2005                                :)
(:Purpose: Test of a General Expression      :)
(:with the operands set as follows          :)
(:  operand1 = Sequence of single element nodes:)
(:  operator = <=:)
(:  operand2 = Sequence of single atomic values:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[1]/hours[1]) <= (10000,50000)
