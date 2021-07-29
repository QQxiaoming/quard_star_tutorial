(:*******************************************************:)
(:Test: fn-years-from-duration-12                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P25Y10M")) idiv fn:years-from-duration(xs:yearMonthDuration("P05Y10M"))
