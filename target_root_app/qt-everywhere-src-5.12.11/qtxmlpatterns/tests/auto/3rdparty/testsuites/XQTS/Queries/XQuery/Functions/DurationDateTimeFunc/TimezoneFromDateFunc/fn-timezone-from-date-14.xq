(:*******************************************************:)
(:Test: timezone-from-date-14                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-date" function   :)
(:as argument to the "fn:string" function.               :) 
(:*******************************************************:)

fn:string(fn:timezone-from-date(xs:date("1970-01-01Z")))