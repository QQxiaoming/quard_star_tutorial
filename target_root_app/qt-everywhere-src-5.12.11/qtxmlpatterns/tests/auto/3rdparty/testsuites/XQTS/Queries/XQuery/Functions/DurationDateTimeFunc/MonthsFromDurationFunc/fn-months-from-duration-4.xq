(:*******************************************************:)
(:Test: fn-months-from-duration-4                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function :)
(:involving a "numeric-less-than" operation (le operator):)
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P21Y10M")) le fn:months-from-duration(xs:yearMonthDuration("P22Y10M"))