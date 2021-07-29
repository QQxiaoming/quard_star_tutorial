(:*******************************************************:)
(:Test: fn-minutes-from-duration-7                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:used as arguments to an avg function.                  :) 
(:*******************************************************:)

fn:avg((fn:minutes-from-duration(xs:dayTimeDuration("P23DT10H20M")),fn:minutes-from-duration(xs:dayTimeDuration("P21DT10H10M")))) 