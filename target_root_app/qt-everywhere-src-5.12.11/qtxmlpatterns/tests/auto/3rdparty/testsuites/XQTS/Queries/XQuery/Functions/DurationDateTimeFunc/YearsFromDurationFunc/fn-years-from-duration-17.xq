(:*******************************************************:)
(:Test: fn-years-from-duration-17                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P22Y10M")) ne fn:years-from-duration(xs:yearMonthDuration("P23Y10M"))
