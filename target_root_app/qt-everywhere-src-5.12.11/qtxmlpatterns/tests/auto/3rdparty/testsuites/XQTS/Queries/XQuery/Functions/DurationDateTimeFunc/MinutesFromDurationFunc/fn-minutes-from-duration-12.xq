(:*******************************************************:)
(:Test: fn-minutes-from-duration-12                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P25DT10H20M")) idiv fn:minutes-from-duration(xs:dayTimeDuration("P05DT02H04M"))
