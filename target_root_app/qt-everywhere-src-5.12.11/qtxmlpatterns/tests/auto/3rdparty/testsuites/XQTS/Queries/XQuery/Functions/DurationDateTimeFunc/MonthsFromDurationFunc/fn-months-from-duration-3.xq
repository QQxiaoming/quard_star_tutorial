(:*******************************************************:)
(:Test: fn-months-from-duration-3                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function :)
(:involving a "numeric-less-than" operation (lt operator):)
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P20Y3M")) lt fn:months-from-duration(xs:yearMonthDuration("P21Y2M"))