#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Setting up no_such_verb plugin build environment...${NC}"

# Check for ARM toolchain
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo -e "${RED}Error: ARM toolchain not found. Please install arm-none-eabi-gcc${NC}"
    exit 1
fi

# Clone or update main submodules
if [ ! -d "libDaisy" ]; then
    echo -e "${YELLOW}Cloning libDaisy...${NC}"
    git clone --depth 1 https://github.com/electro-smith/libDaisy.git
else
    echo -e "${YELLOW}libDaisy already present${NC}"
fi

if [ ! -d "DaisySP" ]; then
    echo -e "${YELLOW}Cloning DaisySP...${NC}"
    git clone --depth 1 https://github.com/electro-smith/DaisySP.git
else
    echo -e "${YELLOW}DaisySP already present${NC}"
fi

echo -e "${YELLOW}Initializing libDaisy nested dependencies...${NC}"
cd libDaisy
git submodule init
git submodule update --remote --depth 1
cd ..

echo -e "${YELLOW}Initializing DaisySP nested dependencies...${NC}"
cd DaisySP
git submodule init
git submodule update --remote --depth 1
cd ..

echo -e "${YELLOW}Applying libDaisy const-correctness patch...${NC}"
cd libDaisy
# Fix const-correctness issues in patched HAL driver
sed -i '' 's/^uint32_t SDMMC_ReadFIFO(SDMMC_TypeDef \*SDMMCx)/uint32_t SDMMC_ReadFIFO(const SDMMC_TypeDef *SDMMCx)/' \
    Drivers/Patched/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
sed -i '' 's/^uint32_t SDMMC_GetPowerState(SDMMC_TypeDef \*SDMMCx)/uint32_t SDMMC_GetPowerState(const SDMMC_TypeDef *SDMMCx)/' \
    Drivers/Patched/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
sed -i '' 's/^HAL_StatusTypeDef SDMMC_SendCommand(SDMMC_TypeDef \*SDMMCx, SDMMC_CmdInitTypeDef \*Command)/HAL_StatusTypeDef SDMMC_SendCommand(SDMMC_TypeDef *SDMMCx, const SDMMC_CmdInitTypeDef *Command)/' \
    Drivers/Patched/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
sed -i '' 's/^uint8_t SDMMC_GetCommandResponse(SDMMC_TypeDef \*SDMMCx)/uint8_t SDMMC_GetCommandResponse(const SDMMC_TypeDef *SDMMCx)/' \
    Drivers/Patched/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
sed -i '' 's/^uint32_t SDMMC_GetResponse(SDMMC_TypeDef \*SDMMCx, uint32_t Response)/uint32_t SDMMC_GetResponse(const SDMMC_TypeDef *SDMMCx, uint32_t Response)/' \
    Drivers/Patched/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
sed -i '' 's/^HAL_StatusTypeDef SDMMC_ConfigData(SDMMC_TypeDef \*SDMMCx, SDMMC_DataInitTypeDef \*Data)/HAL_StatusTypeDef SDMMC_ConfigData(SDMMC_TypeDef *SDMMCx, const SDMMC_DataInitTypeDef *Data)/' \
    Drivers/Patched/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
sed -i '' 's/^uint32_t SDMMC_GetDataCounter(SDMMC_TypeDef \*SDMMCx)/uint32_t SDMMC_GetDataCounter(const SDMMC_TypeDef *SDMMCx)/' \
    Drivers/Patched/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
sed -i '' 's/^uint32_t SDMMC_GetFIFOCount(SDMMC_TypeDef \*SDMMCx)/uint32_t SDMMC_GetFIFOCount(const SDMMC_TypeDef *SDMMCx)/' \
    Drivers/Patched/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
cd ..

echo -e "${YELLOW}Building libDaisy...${NC}"
cd libDaisy
make clean > /dev/null 2>&1 || true
if ! make; then
    echo -e "${RED}libDaisy build failed${NC}"
    exit 1
fi
cd ..

echo -e "${YELLOW}Building DaisySP...${NC}"
cd DaisySP
make clean > /dev/null 2>&1 || true
if ! make; then
    echo -e "${RED}DaisySP build failed${NC}"
    exit 1
fi
cd ..

echo -e "${GREEN}Dependencies built successfully!${NC}"
echo -e "${YELLOW}To build the plugin, run: make${NC}"
