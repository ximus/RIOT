/****************************************************************************************************
 * @fn          cc430_pmm_increase_vcore
 * @brief       Increase vcore to appropriate level (step-by-step)
 * @param       level            uint8_t          Next upper level of vcore
 * @return      none
 ***************************************************************************************************/
void cc430_pmm_increase_vcore (uint8_t level)
{
    // From CC430 Datasheet
    // Open PMM registers for write access
    PMMCTL0_H = 0xA5;
    // Set SVS/SVM high side new level
    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
    // Set SVM low side to new level
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
    // Wait till SVM is settled
    while ((PMMIFG & SVSMLDLYIFG) == 0);
    // Clear already set flags
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
    // Set VCore to new level
    PMMCTL0_L = PMMCOREV0 * level;
    // Wait till new level reached
    if ((PMMIFG & SVMLIFG))
       while ((PMMIFG & SVMLVLRIFG) == 0);
    // Set SVS/SVM low side to new level
    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
    // Lock PMM registers for write access
    PMMCTL0_H = 0x00;
}

/****************************************************************************************************
 * @fn          cc430_pmm_decrease_vcore
 * @brief       level            uint8_t          Next lower level of vcore
 * @param       none
 * @return      none
 ***************************************************************************************************/
void cc430_pmm_decrease_vcore(uint8_t level)
{
    // From CC430 Datasheet
    // Open PMM registers for write access
    PMMCTL0_H = 0xA5;
    // Set SVM & SVS low side to new level
    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
    // Wait till SVM is settled
    while ((PMMIFG & SVSMLDLYIFG) == 0);
    // Set VCore to new level
    PMMCTL0_L = PMMCOREV0 * level;
    // Lock PMM registers for write access
    PMMCTL0_H = 0x00;
}

/****************************************************************************************************
 * @fn          cc430_pmm_set_vcore
 * @brief       Set vcore to target level
 * @param       targetlev        uint8_t          vcore target level
 * @return      none
 ***************************************************************************************************/
void cc430_pmm_set_vcore(uint8_t targetlev)
{
    uint8_t curlev = PMMCTL0_L & PMMCOREV_3;

    // Change VCORE level one step at a time to reach targetlev
    while (curlev != targetlev)
    {
        if (curlev < targetlev)
        {
            curlev++;
            cc430_pmm_increase_vcore(curlev);
        }
        if (curlev > targetlev)
        {
            curlev--;
            cc430_pmm_decrease_vcore(curlev);
        }
    }
}