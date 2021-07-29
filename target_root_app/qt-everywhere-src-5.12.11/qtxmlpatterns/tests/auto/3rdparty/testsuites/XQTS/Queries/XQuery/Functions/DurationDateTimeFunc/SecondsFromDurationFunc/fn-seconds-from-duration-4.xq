(:*******************************************************:)
(:Test: fn-seconds-from-duration-4                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:involving a "numeric-less-than" operation (le operator):)
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P21DT10H10M09S")) le fn:seconds-from-duration(xs:dayTimeDuration("P22DT10H09M31S"))