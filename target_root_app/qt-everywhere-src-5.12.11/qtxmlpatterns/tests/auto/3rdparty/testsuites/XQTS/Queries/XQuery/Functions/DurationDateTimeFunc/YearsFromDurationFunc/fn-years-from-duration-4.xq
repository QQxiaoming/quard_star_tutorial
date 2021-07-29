(:*******************************************************:)
(:Test: fn-years-from-duration-4                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:involving a "numeric-less-than" operation (le operator):)
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P21Y10M")) le fn:years-from-duration(xs:yearMonthDuration("P22Y10M"))