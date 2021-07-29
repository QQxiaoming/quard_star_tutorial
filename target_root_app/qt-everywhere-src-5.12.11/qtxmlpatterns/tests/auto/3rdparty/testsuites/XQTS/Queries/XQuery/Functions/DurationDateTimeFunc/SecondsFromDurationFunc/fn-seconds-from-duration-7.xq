(:*******************************************************:)
(:Test: fn-seconds-from-duration-7                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:used as arguments to an avg function.                  :) 
(:*******************************************************:)

fn:avg((fn:seconds-from-duration(xs:dayTimeDuration("P23DT10H20M30S")),fn:seconds-from-duration(xs:dayTimeDuration("P21DT10H10M32S")))) 