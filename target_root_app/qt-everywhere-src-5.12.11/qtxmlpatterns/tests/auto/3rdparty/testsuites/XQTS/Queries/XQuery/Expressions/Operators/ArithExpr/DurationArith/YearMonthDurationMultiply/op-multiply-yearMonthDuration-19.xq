(:*******************************************************:)
(:Test: op-multiply-yearMonthDuration-19                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "multiply-yearMonthDuration" operator:)
(:with arg2 set to NaN.  Should raise an error           :)
(:*******************************************************:)
 
xs:yearMonthDuration("P13Y09M") * fn:number(())