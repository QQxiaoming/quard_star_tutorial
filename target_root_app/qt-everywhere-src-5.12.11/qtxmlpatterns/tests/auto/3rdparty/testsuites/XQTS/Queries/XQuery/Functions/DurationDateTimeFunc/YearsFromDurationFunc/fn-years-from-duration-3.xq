(:*******************************************************:)
(:Test: fn-years-from-duration-3                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:involving a "numeric-less-than" operation (lt operator):)
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P20Y3M")) lt fn:years-from-duration(xs:yearMonthDuration("P21Y2M"))