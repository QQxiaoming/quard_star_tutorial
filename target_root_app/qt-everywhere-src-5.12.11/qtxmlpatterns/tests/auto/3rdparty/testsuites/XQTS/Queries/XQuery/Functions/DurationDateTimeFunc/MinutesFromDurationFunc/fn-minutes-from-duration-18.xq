(:*******************************************************:)
(:Test: fn-minutes-from-duration-18                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P20DT03H09M")) le fn:minutes-from-duration(xs:dayTimeDuration("P21DT15H21M"))