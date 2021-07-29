(:*******************************************************:)
(:Test: fn-minutes-from-duration-4                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:involving a "numeric-less-than" operation (le operator):)
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P21DT10H10M")) le fn:minutes-from-duration(xs:dayTimeDuration("P22DT10H09M"))