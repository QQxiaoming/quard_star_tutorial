(:*******************************************************:)
(:Test: fn-months-from-duration-7                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function :)
(:used as arguments to an avg function.                  :) 
(:*******************************************************:)

fn:avg((fn:months-from-duration(xs:yearMonthDuration("P23Y10M")),fn:months-from-duration(xs:yearMonthDuration("P21Y10M")))) 