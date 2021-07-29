(:*******************************************************:)
(:Test: fn-years-from-duration-18                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P21Y01M")) le fn:years-from-duration(xs:yearMonthDuration("P21Y15M"))