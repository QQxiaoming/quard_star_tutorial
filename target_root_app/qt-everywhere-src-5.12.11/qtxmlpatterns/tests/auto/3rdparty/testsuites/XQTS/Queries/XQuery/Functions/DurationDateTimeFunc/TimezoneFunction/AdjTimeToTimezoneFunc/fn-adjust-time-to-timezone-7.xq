(:*******************************************************:)
(:Test: adjust-time-to-timezone-7                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The "adjust-time-to-timezone" function   :)
(:as per example 7 (for this function) of the F&O  specs. :)
(:*******************************************************:)

fn:adjust-time-to-timezone(xs:time("10:00:00-07:00"), xs:dayTimeDuration("PT10H"))