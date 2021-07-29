(:*******************************************************:)
(:Test: fn-years-from-duration-7                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:used as arguments to an avg function.                  :) 
(:*******************************************************:)

fn:avg((fn:years-from-duration(xs:yearMonthDuration("P23Y10M")),fn:years-from-duration(xs:yearMonthDuration("P21Y10M")))) 